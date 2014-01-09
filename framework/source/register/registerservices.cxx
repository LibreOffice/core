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
#include <services/desktop.hxx>
#include <jobs/jobexecutor.hxx>
#include <uifactory/uielementfactorymanager.hxx>
#include <uifactory/uicontrollerfactory.hxx>
#include <uielement/uicommanddescription.hxx>
#include <uiconfiguration/moduleuicfgsupplier.hxx>
#include <uifactory/menubarfactory.hxx>
#include <accelerators/moduleacceleratorconfiguration.hxx>
#include <uifactory/toolboxfactory.hxx>
#include "uiconfiguration/windowstateconfiguration.hxx"
#include <services/autorecovery.hxx>
#include <uifactory/statusbarfactory.hxx>
#include <uiconfiguration/uicategorydescription.hxx>
#include <services/sessionlistener.hxx>
#include <services/taskcreatorsrv.hxx>

#include <uifactory/windowcontentfactorymanager.hxx>
#include <services/substitutepathvars.hxx>
#include <services/pathsettings.hxx>
#include <services/ContextChangeEventMultiplexer.hxx>

COMPONENTGETFACTORY ( fwk,
                        IFFACTORY( ::framework::Desktop                                 )   else
                        IFFACTORY( ::framework::JobExecutor                             )   else
                        IFFACTORY( ::framework::UIElementFactoryManager                 )   else
                        IFFACTORY( ::framework::PopupMenuControllerFactory              )   else
                        IFFACTORY( ::framework::UICommandDescription                    )   else
                        IFFACTORY( ::framework::ModuleUIConfigurationManagerSupplier    )   else
                        IFFACTORY( ::framework::MenuBarFactory                          )   else
                        IFFACTORY( ::framework::ModuleAcceleratorConfiguration          )   else
                        IFFACTORY( ::framework::ToolBoxFactory                          )   else
                        IFFACTORY( ::framework::WindowStateConfiguration                )   else
                        IFFACTORY( ::framework::ToolbarControllerFactory                )   else
                        IFFACTORY( ::framework::AutoRecovery                            )   else
                        IFFACTORY( ::framework::StatusBarFactory                        )   else
                        IFFACTORY( ::framework::UICategoryDescription                   )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::StatusbarControllerFactory              )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::TaskCreatorService                      )   else
                        IFFACTORY( ::framework::WindowContentFactoryManager             )   else
                        IFFACTORY( ::framework::SubstitutePathVariables                 )   else
                        IFFACTORY( ::framework::PathSettings                            )   else
                        IFFACTORY( ::framework::ContextChangeEventMultiplexer           )
            )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
