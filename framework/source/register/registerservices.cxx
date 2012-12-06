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
#include <services/urltransformer.hxx>
#include <services/desktop.hxx>
#include <services/tabwindowservice.hxx>
#include <services/frame.hxx>
#include <services/modulemanager.hxx>
//#include <dispatch/oxt_handler.hxx>
#include <jobs/jobexecutor.hxx>
//#include <recording/dispatchrecordersupplier.hxx>
//#include <recording/dispatchrecorder.hxx>
//#include <dispatch/mailtodispatcher.hxx>
//#include <dispatch/servicehandler.hxx>
#include <jobs/jobdispatch.hxx>
#include <services/backingcomp.hxx>
//#include <services/dispatchhelper.hxx>
#include <services/layoutmanager.hxx>
//#include <services/license.hxx>
#include <uifactory/uielementfactorymanager.hxx>
#include <uifactory/popupmenucontrollerfactory.hxx>
//#include <uielement/fontmenucontroller.hxx>
//#include <uielement/fontsizemenucontroller.hxx>
#include <uielement/objectmenucontroller.hxx>
//#include <uielement/headermenucontroller.hxx>
//#include <uielement/footermenucontroller.hxx>
#include <uielement/controlmenucontroller.hxx>
//#include <uielement/macrosmenucontroller.hxx>
#include <uielement/uicommanddescription.hxx>
#include <uiconfiguration/uiconfigurationmanager.hxx>
#include <uiconfiguration/moduleuicfgsupplier.hxx>
#include <uiconfiguration/moduleuiconfigurationmanager.hxx>
#include <uifactory/menubarfactory.hxx>
#include <accelerators/globalacceleratorconfiguration.hxx>
#include <accelerators/moduleacceleratorconfiguration.hxx>
#include <accelerators/documentacceleratorconfiguration.hxx>
#include <uifactory/toolboxfactory.hxx>
#include <uifactory/addonstoolboxfactory.hxx>
#include "uiconfiguration/windowstateconfiguration.hxx"
//#include <uielement/toolbarsmenucontroller.hxx>
#include "uifactory/toolbarcontrollerfactory.hxx"
#include "uifactory/statusbarcontrollerfactory.hxx"
#include <services/autorecovery.hxx>
#include <helper/statusindicatorfactory.hxx>
#include <uielement/recentfilesmenucontroller.hxx>
#include <uifactory/statusbarfactory.hxx>
#include <uiconfiguration/uicategorydescription.hxx>
#include <services/sessionlistener.hxx>
//#include <uielement/newmenucontroller.hxx>
#include <services/taskcreatorsrv.hxx>
//#include <services/uriabbreviation.hxx>

#include <uielement/langselectionstatusbarcontroller.hxx>
//#include <uielement/langselectionmenucontroller.hxx>
#include <uiconfiguration/imagemanager.hxx>
#include <uifactory/windowcontentfactorymanager.hxx>
#include <services/substitutepathvars.hxx>
#include <services/pathsettings.hxx>

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTGETFACTORY (   IFFACTORY( ::framework::URLTransformer                          )   else
                        IFFACTORY( ::framework::Desktop                                 )   else
                        IFFACTORY( ::framework::Frame                                   )   else
                        //IFFACTORY( ::framework::Oxt_Handler                             )   else
                        IFFACTORY( ::framework::JobExecutor                             )   else
                        //IFFACTORY( ::framework::DispatchRecorderSupplier                )   else
                        //IFFACTORY( ::framework::DispatchRecorder                        )   else
                        //IFFACTORY( ::framework::MailToDispatcher                        )   else
                        //IFFACTORY( ::framework::ServiceHandler                          )   else
                        IFFACTORY( ::framework::JobDispatch                             )   else
                        IFFACTORY( ::framework::BackingComp                             )   else
                        //IFFACTORY( ::framework::DispatchHelper                          )   else
                        IFFACTORY( ::framework::LayoutManager                           )   else
                        //IFFACTORY( ::framework::License                                 )   else
                        IFFACTORY( ::framework::UIElementFactoryManager                 )   else
                        IFFACTORY( ::framework::PopupMenuControllerFactory              )   else
                        //IFFACTORY( ::framework::FontMenuController                      )   else
                        //IFFACTORY( ::framework::FontSizeMenuController                  )   else
                        IFFACTORY( ::framework::ObjectMenuController                    )   else
                        //IFFACTORY( ::framework::HeaderMenuController                    )   else
                        //IFFACTORY( ::framework::FooterMenuController                    )   else
                        IFFACTORY( ::framework::ControlMenuController                   )   else
                        //IFFACTORY( ::framework::MacrosMenuController                    )   else
                        IFFACTORY( ::framework::UICommandDescription                    )   else
                        IFFACTORY( ::framework::ModuleManager                           )   else
                        IFFACTORY( ::framework::UIConfigurationManager                  )   else
                        IFFACTORY( ::framework::ModuleUIConfigurationManagerSupplier    )   else
                        IFFACTORY( ::framework::ModuleUIConfigurationManager            )   else
                        IFFACTORY( ::framework::MenuBarFactory                          )   else
                        IFFACTORY( ::framework::GlobalAcceleratorConfiguration          )   else
                        IFFACTORY( ::framework::ModuleAcceleratorConfiguration          )   else
                        IFFACTORY( ::framework::DocumentAcceleratorConfiguration        )   else
                        IFFACTORY( ::framework::ToolBoxFactory                          )   else
                        IFFACTORY( ::framework::AddonsToolBoxFactory                    )   else
                        IFFACTORY( ::framework::WindowStateConfiguration                )   else
                        IFFACTORY( ::framework::ToolbarControllerFactory                )   else
                        //IFFACTORY( ::framework::ToolbarsMenuController                  ) else
                        IFFACTORY( ::framework::AutoRecovery                            )   else
                        IFFACTORY( ::framework::StatusIndicatorFactory                  )   else
                        IFFACTORY( ::framework::RecentFilesMenuController               )   else
                        IFFACTORY( ::framework::StatusBarFactory                        )   else
                        IFFACTORY( ::framework::UICategoryDescription                   )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::StatusbarControllerFactory              )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::TaskCreatorService                      )   else
                        //IFFACTORY( ::framework::NewMenuController                       )   else
                        //IFFACTORY( ::framework::UriAbbreviation                         )   else
                        //IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::ImageManager                            )   else
                        IFFACTORY( ::framework::LangSelectionStatusbarController        )   else
                        //IFFACTORY( ::framework::LanguageSelectionMenuController         )   else
                        IFFACTORY( ::framework::WindowContentFactoryManager             )   else
                        IFFACTORY( ::framework::TabWindowService                        )   else
                        IFFACTORY( ::framework::SubstitutePathVariables                 )   else
                        IFFACTORY( ::framework::PathSettings                            )
            )

