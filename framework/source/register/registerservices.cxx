/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <macros/registration.hxx>

/*=================================================================================================================
    Add new include and new register info to for new services.

    Example:

        #include <service1.hxx>
        #include <service2.hxx>

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
#include <services/layoutmanager.hxx>
#include <uifactory/uielementfactorymanager.hxx>
#include <uifactory/uicontrollerfactory.hxx>
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
#include <uielement/popuptoolbarcontroller.hxx>

COMPONENTGETFACTORY ( fwk,
                        IFFACTORY( ::framework::URLTransformer                          )   else
                        IFFACTORY( ::framework::Desktop                                 )   else
                        IFFACTORY( ::framework::Frame                                   )   else
                        IFFACTORY( ::framework::JobExecutor                             )   else
                        IFFACTORY( ::framework::JobDispatch                             )   else
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
                        IFFACTORY( ::framework::ContextChangeEventMultiplexer           )   else
                        IFFACTORY( ::framework::OpenToolbarController                   )   else
                        IFFACTORY( ::framework::NewToolbarController                    )   else
                        IFFACTORY( ::framework::WizardsToolbarController                )
            )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
