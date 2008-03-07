/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:52:48 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
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

#ifndef __FRAMEWORK_SERVICES_URLTRANSFORMER_HXX_
#include <services/urltransformer.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
#include <services/desktop.hxx>
#endif


#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_
#include <services/modulemanager.hxx>
#endif

#ifndef __FRAMEWORK_JOBS_JOBEXECUTOR_HXX_
#include <jobs/jobexecutor.hxx>
#endif

#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDERSUPPLIER_HXX_
#include <recording/dispatchrecordersupplier.hxx>
#endif

#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDER_HXX_
#include <recording/dispatchrecorder.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_MAILTODISPATCHER_HXX_
#include <dispatch/mailtodispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_SERVICEHANDLER_HXX_
#include <dispatch/servicehandler.hxx>
#endif

#ifndef __FRAMEWORK_JOBS_JOBDISPATCH_HXX_
#include <jobs/jobdispatch.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_BACKINGCOMP_HXX_
#include <services/backingcomp.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_DISPATCHHELPER_HXX_
#include <services/dispatchhelper.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
#include <services/layoutmanager.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_LICENSE_HXX_
#include <services/license.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX_
#include <uifactory/uielementfactorymanager.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_POPUPMENUCONTROLLERFACTORY_HXX_
#include <uifactory/popupmenucontrollerfactory.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_FONTMENUCONTROLLER_HXX_
#include <uielement/fontmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_FONTSIZEMENUCONTROLLER_HXX_
#include <uielement/fontsizemenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_OBJECTMENUCONTROLLER_HXX_
#include <uielement/objectmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_HEADERMENUCONTROLLER_HXX_
#include <uielement/headermenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_FOOTERMENUCONTROLLER_HXX_
#include <uielement/footermenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_CONTROLMENUCONTROLLER_HXX_
#include <uielement/controlmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_MACROSMENUCONTROLLER_HXX_
#include <uielement/macrosmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_UICOMMANDDESCRIPTION_HXX_
#include <uielement/uicommanddescription.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_UICONFIGMANAGER_HXX_
#include <uiconfiguration/uiconfigurationmanager.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_
#include <uiconfiguration/moduleuicfgsupplier.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_MODULEUICONFIGMANAGER_HXX_
#include <uiconfiguration/moduleuiconfigurationmanager.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_MENUBARFACTORY_HXX_
#include <uifactory/menubarfactory.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_
#include <accelerators/globalacceleratorconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
#include <accelerators/moduleacceleratorconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_
#include <accelerators/documentacceleratorconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_TOOLBOXFACTORY_HXX_
#include <uifactory/toolboxfactory.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_ADDONSTOOLBOXFACTORY_HXX_
#include <uifactory/addonstoolboxfactory.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#include "uiconfiguration/windowstateconfiguration.hxx"
#endif

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_
#include <uielement/toolbarsmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_TOOLBARCONTROLLERFACTORY_HXX_
#include "uifactory/toolbarcontrollerfactory.hxx"
#endif

#ifndef __FRAMEWORK_UIFACTORY_STATUSBARCONTROLLERFACTORY_HXX_
#include "uifactory/statusbarcontrollerfactory.hxx"
#endif

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_
#include <uielement/toolbarsmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_AUTORECOVERY_HXX_
#include <services/autorecovery.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#include <helper/statusindicatorfactory.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_RECENTFILESMENUCONTROLLER_HXX_
#include <uielement/recentfilesmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIFACTORY_STATUSBARFACTORY_HXX_
#include <uifactory/statusbarfactory.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_UICATEGORYDESCRPTION_HXX_
#include <uiconfiguration/uicategorydescription.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_SESSIONLISTENER_HXX_
#include <services/sessionlistener.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_LOGOIMAGESTATUSBARCONTROLLER_HXX_
#include <uielement/logoimagestatusbarcontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_LOGOTEXTSTATUSBARCONTROLLER_HXX_
#include <uielement/logotextstatusbarcontroller.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_
#include <uielement/newmenucontroller.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
#include <services/taskcreatorsrv.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_SIMPLETEXTSTATUSBARCONTROLLER_HXX_
#include <uielement/simpletextstatusbarcontroller.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_
#include <services/uriabbreviation.hxx>
#endif

#include <dispatch/popupmenudispatcher.hxx>

#include <uielement/langselectionstatusbarcontroller.hxx>
#include <uielement/langselectionmenucontroller.hxx>

#include <uiconfiguration/imagemanager.hxx>

COMPONENTGETIMPLEMENTATIONENVIRONMENT

COMPONENTWRITEINFO  (   COMPONENTINFO( ::framework::URLTransformer                          )
                        COMPONENTINFO( ::framework::Desktop                                 )
                        COMPONENTINFO( ::framework::Frame                                   )
                        COMPONENTINFO( ::framework::JobExecutor                             )
                        COMPONENTINFO( ::framework::DispatchRecorderSupplier                )
                        COMPONENTINFO( ::framework::DispatchRecorder                        )
                        COMPONENTINFO( ::framework::MailToDispatcher                        )
                        COMPONENTINFO( ::framework::ServiceHandler                          )
                        COMPONENTINFO( ::framework::JobDispatch                             )
                        COMPONENTINFO( ::framework::BackingComp                             )
                        COMPONENTINFO( ::framework::DispatchHelper                          )
                        COMPONENTINFO( ::framework::LayoutManager                           )
                        COMPONENTINFO( ::framework::License                                 )
                        COMPONENTINFO( ::framework::UIElementFactoryManager                 )
                        COMPONENTINFO( ::framework::PopupMenuControllerFactory              )
                        COMPONENTINFO( ::framework::FontMenuController                      )
                        COMPONENTINFO( ::framework::FontSizeMenuController                  )
                        COMPONENTINFO( ::framework::ObjectMenuController                    )
                        COMPONENTINFO( ::framework::HeaderMenuController                    )
                        COMPONENTINFO( ::framework::FooterMenuController                    )
                        COMPONENTINFO( ::framework::ControlMenuController                   )
                        COMPONENTINFO( ::framework::MacrosMenuController                    )
                        COMPONENTINFO( ::framework::UICommandDescription                    )
                        COMPONENTINFO( ::framework::ModuleManager                           )
                        COMPONENTINFO( ::framework::UIConfigurationManager                  )
                        COMPONENTINFO( ::framework::ModuleUIConfigurationManagerSupplier    )
                        COMPONENTINFO( ::framework::ModuleUIConfigurationManager            )
                        COMPONENTINFO( ::framework::MenuBarFactory                          )
                        COMPONENTINFO( ::framework::GlobalAcceleratorConfiguration          )
                        COMPONENTINFO( ::framework::ModuleAcceleratorConfiguration          )
                        COMPONENTINFO( ::framework::DocumentAcceleratorConfiguration        )
                        COMPONENTINFO( ::framework::ToolBoxFactory                          )
                        COMPONENTINFO( ::framework::AddonsToolBoxFactory                    )
                        COMPONENTINFO( ::framework::WindowStateConfiguration                )
                        COMPONENTINFO( ::framework::ToolbarControllerFactory                )
                        COMPONENTINFO( ::framework::ToolbarsMenuController                  )
                        COMPONENTINFO( ::framework::AutoRecovery                            )
                        COMPONENTINFO( ::framework::StatusIndicatorFactory                  )
                        COMPONENTINFO( ::framework::RecentFilesMenuController               )
                        COMPONENTINFO( ::framework::StatusBarFactory                        )
                        COMPONENTINFO( ::framework::UICategoryDescription                   )
                        COMPONENTINFO( ::framework::StatusbarControllerFactory              )
                        COMPONENTINFO( ::framework::SessionListener                         )
                        COMPONENTINFO( ::framework::LogoImageStatusbarController            )
                        COMPONENTINFO( ::framework::LogoTextStatusbarController             )
                        COMPONENTINFO( ::framework::NewMenuController                       )
                        COMPONENTINFO( ::framework::TaskCreatorService                      )
                        COMPONENTINFO( ::framework::SimpleTextStatusbarController           )
                        COMPONENTINFO( ::framework::UriAbbreviation                         )
                        COMPONENTINFO( ::framework::PopupMenuDispatcher                     )
                        COMPONENTINFO( ::framework::ImageManager                            )
                        COMPONENTINFO( ::framework::LangSelectionStatusbarController        )
                        COMPONENTINFO( ::framework::LanguageSelectionMenuController         )
                    )

COMPONENTGETFACTORY (   IFFACTORY( ::framework::URLTransformer                          )   else
                        IFFACTORY( ::framework::Desktop                                 )   else
                        IFFACTORY( ::framework::Frame                                   )   else
                        IFFACTORY( ::framework::JobExecutor                             )   else
                        IFFACTORY( ::framework::DispatchRecorderSupplier                )   else
                        IFFACTORY( ::framework::DispatchRecorder                        )   else
                        IFFACTORY( ::framework::MailToDispatcher                        )   else
                        IFFACTORY( ::framework::ServiceHandler                          )   else
                        IFFACTORY( ::framework::JobDispatch                             )   else
                        IFFACTORY( ::framework::BackingComp                             )   else
                        IFFACTORY( ::framework::DispatchHelper                          )   else
                        IFFACTORY( ::framework::LayoutManager                           )   else
                        IFFACTORY( ::framework::License                                 )   else
                        IFFACTORY( ::framework::UIElementFactoryManager                 )   else
                        IFFACTORY( ::framework::PopupMenuControllerFactory              )   else
                        IFFACTORY( ::framework::FontMenuController                      )   else
                        IFFACTORY( ::framework::FontSizeMenuController                  )   else
                        IFFACTORY( ::framework::ObjectMenuController                    )   else
                        IFFACTORY( ::framework::HeaderMenuController                    )   else
                        IFFACTORY( ::framework::FooterMenuController                    )   else
                        IFFACTORY( ::framework::ControlMenuController                   )   else
                        IFFACTORY( ::framework::MacrosMenuController                    )   else
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
                        IFFACTORY( ::framework::ToolbarsMenuController                  )   else
                        IFFACTORY( ::framework::AutoRecovery                            )   else
                        IFFACTORY( ::framework::StatusIndicatorFactory                  )   else
                        IFFACTORY( ::framework::RecentFilesMenuController               )   else
                        IFFACTORY( ::framework::StatusBarFactory                        )   else
                        IFFACTORY( ::framework::UICategoryDescription                   )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::StatusbarControllerFactory              )   else
                        IFFACTORY( ::framework::SessionListener                         )   else
                        IFFACTORY( ::framework::LogoImageStatusbarController            )   else
                        IFFACTORY( ::framework::LogoTextStatusbarController             )   else
                        IFFACTORY( ::framework::TaskCreatorService                      )   else
                        IFFACTORY( ::framework::NewMenuController                       )   else
                        IFFACTORY( ::framework::SimpleTextStatusbarController           )   else
                        IFFACTORY( ::framework::UriAbbreviation                         )   else
                        IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::ImageManager                            )
                        IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::LangSelectionStatusbarController        )   else
                        IFFACTORY( ::framework::LanguageSelectionMenuController         )
            )

