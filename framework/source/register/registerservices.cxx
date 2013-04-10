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
#include <jobs/jobexecutor.hxx>
#include <jobs/jobdispatch.hxx>
#include <services/backingcomp.hxx>
#include <services/layoutmanager.hxx>
#include <uifactory/uielementfactorymanager.hxx>
#include <uifactory/popupmenucontrollerfactory.hxx>
#include <uielement/objectmenucontroller.hxx>
#include <uielement/controlmenucontroller.hxx>
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
#include "uifactory/toolbarcontrollerfactory.hxx"
#include "uifactory/statusbarcontrollerfactory.hxx"
#include <services/autorecovery.hxx>
#include <helper/statusindicatorfactory.hxx>
#include <uielement/recentfilesmenucontroller.hxx>
#include <uifactory/statusbarfactory.hxx>
#include <uiconfiguration/uicategorydescription.hxx>
#include <services/sessionlistener.hxx>
#include <services/taskcreatorsrv.hxx>
#include <uielement/langselectionstatusbarcontroller.hxx>
#include <uiconfiguration/imagemanager.hxx>
#include <uifactory/windowcontentfactorymanager.hxx>
#include <services/substitutepathvars.hxx>
#include <services/pathsettings.hxx>
#include <services/ContextChangeEventMultiplexer.hxx>

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTGETFACTORY (   IFFACTORY( ::framework::URLTransformer                          )   else
                        IFFACTORY( ::framework::Desktop                                 )   else
                        IFFACTORY( ::framework::Frame                                   )   else
                        IFFACTORY( ::framework::JobExecutor                             )   else
                        IFFACTORY( ::framework::JobDispatch                             )   else
                        IFFACTORY( ::framework::BackingComp                             )   else
                        IFFACTORY( ::framework::LayoutManager                           )   else
                        IFFACTORY( ::framework::UIElementFactoryManager                 )   else
                        IFFACTORY( ::framework::PopupMenuControllerFactory              )   else
                        IFFACTORY( ::framework::ObjectMenuController                    )   else
                        IFFACTORY( ::framework::ControlMenuController                   )   else
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
                        IFFACTORY( ::framework::AutoRecovery                            )   else
                        IFFACTORY( ::framework::StatusIndicatorFactory                  )   else
                        IFFACTORY( ::framework::RecentFilesMenuController               )   else
                        IFFACTORY( ::framework::StatusBarFactory                        )   else
                        IFFACTORY( ::framework::UICategoryDescription                   )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::StatusbarControllerFactory              )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::TaskCreatorService                      )   else
                        IFFACTORY( ::framework::ImageManager                            )   else
                        IFFACTORY( ::framework::LangSelectionStatusbarController        )   else
                        IFFACTORY( ::framework::WindowContentFactoryManager             )   else
                        IFFACTORY( ::framework::TabWindowService                        )   else
                        IFFACTORY( ::framework::SubstitutePathVariables                 )   else
                        IFFACTORY( ::framework::PathSettings                            )   else
                        IFFACTORY( ::framework::ContextChangeEventMultiplexer           )
            )

