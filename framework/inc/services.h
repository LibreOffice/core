/*************************************************************************
 *
 *  $RCSfile: services.h,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:14:49 $
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
 *  along with this library; if not, write to the Free Software
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

#ifndef __FRAMEWORK_SERVICES_H_
#define __FRAMEWORK_SERVICES_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  used servicenames by framework
//_________________________________________________________________________________________________________________

#define SERVICENAME_FRAME                                       DECLARE_ASCII("com.sun.star.frame.Frame"                            )
#define SERVICENAME_DESKTOP                                     DECLARE_ASCII("com.sun.star.frame.Desktop"                          )
#define SERVICENAME_TASK                                        DECLARE_ASCII("com.sun.star.frame.Task"                             )
#define SERVICENAME_FRAMELOADERFACTORY                          DECLARE_ASCII("com.sun.star.frame.FrameLoaderFactory"               )
#define SERVICENAME_FILTERFACTORY                               DECLARE_ASCII("com.sun.star.document.FilterFactory"                 )
#define SERVICENAME_URLTRANSFORMER                              DECLARE_ASCII("com.sun.star.util.URLTransformer"                    )
#define SERVICENAME_PLUGINFRAME                                 DECLARE_ASCII("com.sun.star.mozilla.Plugin"                         )
#define SERVICENAME_BRIDGEFACTORY                               DECLARE_ASCII("com.sun.star.bridge.BridgeFactory"                   )
#define SERVICENAME_INIMANAGER                                  DECLARE_ASCII("com.sun.star.config.INIManager"                      )
#define SERVICENAME_MEDIATYPEDETECTIONHELPER                    DECLARE_ASCII("com.sun.star.frame.MediaTypeDetectionHelper"         )
#define SERVICENAME_LOGINDIALOG                                 DECLARE_ASCII("com.sun.star.framework.LoginDialog"                  )
#define SERVICENAME_CONFIGURATION                               DECLARE_ASCII("com.sun.star.configuration.ConfigurationRegistry"    )
#define SERVICENAME_VCLTOOLKIT                                  DECLARE_ASCII("com.sun.star.awt.Toolkit"                            )
#define SERVICENAME_SIMPLEREGISTRY                              DECLARE_ASCII("com.sun.star.registry.SimpleRegistry"                )
#define SERVICENAME_NESTEDREGISTRY                              DECLARE_ASCII("com.sun.star.registry.NestedRegistry"                )
#define SERVICENAME_GENERICFRAMELOADER                          DECLARE_ASCII("com.sun.star.comp.office.FilterDetect"               )
#define SERVICENAME_TYPEDETECTION                               DECLARE_ASCII("com.sun.star.document.TypeDetection"                 )
#define SERVICENAME_DOCUMENTPROPERTIES                          DECLARE_ASCII("com.sun.star.document.DocumentProperties"            )
#define SERVICENAME_CFGREGISTRY                                 DECLARE_ASCII("com.sun.star.configuration.ConfigurationRegistry"    )   // describe simple registry service for read access to configuration
#define SERVICENAME_CFGPROVIDER                                 DECLARE_ASCII("com.sun.star.configuration.ConfigurationProvider"    )   // describe cfg-provider service for fatoffice, network installation
#define SERVICENAME_ADMINPROVIDER                               DECLARE_ASCII("com.sun.star.configuration.AdministrationProvider"   )   // describe cfg-provider service for sun webtop
#define SERVICENAME_CFGUPDATEACCESS                             DECLARE_ASCII("com.sun.star.configuration.ConfigurationUpdateAccess")   // provides read/write access to the configuration
#define SERVICENAME_CFGREADACCESS                               DECLARE_ASCII("com.sun.star.configuration.ConfigurationAccess"      )   // provides readonly access to the configuration
#define SERVICENAME_CONTENTHANDLER                              DECLARE_ASCII("com.sun.star.frame.ContentHandler"                   )
#define SERVICENAME_CONTENTHANDLERFACTORY                       DECLARE_ASCII("com.sun.star.frame.ContentHandlerFactory"            )
#define SERVICENAME_APPDISPATCHPROVIDER                         DECLARE_ASCII("com.sun.star.comp.sfx2.AppDispatchProvider"          )
#define SERVICENAME_UCBCONTENTBROKER                            DECLARE_ASCII("com.sun.star.ucb.UniversalContentBroker"             )
#define SERVICENAME_STATUSINDICATOR                             DECLARE_ASCII("com.sun.star.task.XStatusIndicator"                  )
#define SERVICENAME_UIINTERACTIONHANDLER                        DECLARE_ASCII("com.sun.star.task.InteractionHandler"                )
#define SERVICENAME_JOBEXECUTOR                                 DECLARE_ASCII("com.sun.star.task.JobExecutor"                       )
#define SERVICENAME_DISPATCHRECORDERSUPPLIER                    DECLARE_ASCII("com.sun.star.frame.DispatchRecorderSupplier"         )
#define SERVICENAME_DISPATCHRECORDER                            DECLARE_ASCII("com.sun.star.frame.DispatchRecorder"                 )
#define SERVICENAME_SYSTEMSHELLEXECUTE                          DECLARE_ASCII("com.sun.star.system.SystemShellExecute"              )
#define SERVICENAME_PROTOCOLHANDLER                             DECLARE_ASCII("com.sun.star.frame.ProtocolHandler"                  )
#define SERVICENAME_SUBSTITUTEPATHVARIABLES                     DECLARE_ASCII("com.sun.star.util.PathSubstitution"                  )
#define SERVICENAME_PATHSETTINGS                                DECLARE_ASCII("com.sun.star.util.PathSettings"                      )
#define SERVICENAME_DISPATCHHELPER                              DECLARE_ASCII("com.sun.star.frame.DispatchHelper"                   )
#define SERVICENAME_DETECTORFACTORY                             DECLARE_ASCII("com.sun.star.document.ExtendedTypeDetectionFactory"  )
#define SERVICENAME_SCRIPTCONVERTER                             DECLARE_ASCII("com.sun.star.script.Converter"                       )
#define SERVICENAME_LAYOUTMANAGER                               DECLARE_ASCII("drafts.com.sun.star.frame.LayoutManager"             )
#define SERVICENAME_POPUPMENUCONTROLLERFACTORY                  DECLARE_ASCII("drafts.com.sun.star.frame.PopupMenuControllerFactory" )
#define SERVICENAME_POPUPMENUCONTROLLER                         DECLARE_ASCII("drafts.com.sun.star.frame.PopupMenuController"       )
#define SERVICENAME_UICOMMANDDESCRIPTION                        DECLARE_ASCII("drafts.com.sun.star.frame.UICommandDescription"      )
#define SERVICENAME_UIELEMENTFACTORYMANAGER                     DECLARE_ASCII("drafts.com.sun.star.ui.UIElementFactoryManager"      )
#define SERVICENAME_MODULEMANAGER                               DECLARE_ASCII("drafts.com.sun.star.frame.ModuleManager"             )
#define SERVICENAME_UICONFIGURATIONMANAGER                      DECLARE_ASCII("drafts.com.sun.star.ui.UIConfigurationManager"       )
#define SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER        DECLARE_ASCII("drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier" )
#define SERVICENAME_MODULEUICONFIGURATIONMANAGER                DECLARE_ASCII("drafts.com.sun.star.ui.ModuleUIConfigurationManager" )
#define SERVICENAME_MENUBARFACTORY                              DECLARE_ASCII("drafts.com.sun.star.ui.UIElementFactory"             )
#define SERVICENAME_WINDOWSTATECONFIGURATION                    DECLARE_ASCII("drafts.com.sun.star.ui.WindowStateConfiguration"     )
#define SERVICENAME_STARTMODULE                                 DECLARE_ASCII("com.sun.star.frame.StartModule"                      )
#define SERVICENAME_FRAMECONTROLLER                             DECLARE_ASCII("com.sun.star.frame.Controller"                       )
#define SERVICENAME_TOOLBARFACTORY                              DECLARE_ASCII("drafts.com.sun.star.ui.ToolBarFactory"               )
#define SERVICENAME_TOOLBARCONTROLLERFACTORY                    DECLARE_ASCII("com.sun.star.frame.ToolBarControllerFactory"         )
#define SERVICENAME_LICENSE                                     DECLARE_ASCII("com.sun.star.task.Job"                       )

//_________________________________________________________________________________________________________________
//  used implementationnames by framework
//_________________________________________________________________________________________________________________

#define IMPLEMENTATIONNAME_FRAME                                DECLARE_ASCII("com.sun.star.comp.framework.Frame"                   )
#define IMPLEMENTATIONNAME_DESKTOP                              DECLARE_ASCII("com.sun.star.comp.framework.Desktop"                 )
#define IMPLEMENTATIONNAME_TASK                                 DECLARE_ASCII("com.sun.star.comp.framework.Task"                    )
#define IMPLEMENTATIONNAME_FRAMELOADERFACTORY                   DECLARE_ASCII("com.sun.star.comp.framework.FrameLoaderFactory"      )
#define IMPLEMENTATIONNAME_FILTERFACTORY                        DECLARE_ASCII("com.sun.star.comp.framework.FilterFactory"           )
#define IMPLEMENTATIONNAME_URLTRANSFORMER                       DECLARE_ASCII("com.sun.star.comp.framework.URLTransformer"          )
#define IMPLEMENTATIONNAME_PLUGINFRAME                          DECLARE_ASCII("com.sun.star.comp.framework.PluginFrame"             )
#define IMPLEMENTATIONNAME_GENERICFRAMELOADER                   DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"                )
#define IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER             DECLARE_ASCII("com.sun.star.comp.framework.MediaTypeDetectionHelper")
#define IMPLEMENTATIONNAME_LOGINDIALOG                          DECLARE_ASCII("com.sun.star.comp.framework.LoginDialog"             )
#define IMPLEMENTATIONNAME_TYPEDETECTION                        DECLARE_ASCII("com.sun.star.comp.framework.TypeDetection"           )
#define IMPLEMENTATIONNAME_DOCUMENTPROPERTIES                   DECLARE_ASCII("com.sun.star.comp.framework.DocumentProperties"      )
#define IMPLEMENTATIONNAME_SOUNDHANDLER                         DECLARE_ASCII("com.sun.star.comp.framework.SoundHandler"            )
#define IMPLEMENTATIONNAME_CONTENTHANDLERFACTORY                DECLARE_ASCII("com.sun.star.comp.framework.ContentHandlerFactory"   )
#define IMPLEMENTATIONNAME_PIPETERMINATOR                       DECLARE_ASCII("com.sun.star.comp.OfficeIPCThreadController"         )
#define IMPLEMENTATIONNAME_QUICKLAUNCHER                        DECLARE_ASCII("com.sun.star.comp.desktop.QuickstartWrapper"         )
#define IMPLEMENTATIONNAME_JOBEXECUTOR                          DECLARE_ASCII("com.sun.star.comp.framework.JobExecutor"             )
#define IMPLEMENTATIONNAME_DISPATCHRECORDERSUPPLIER             DECLARE_ASCII("com.sun.star.comp.framework.DispatchRecorderSupplier")
#define IMPLEMENTATIONNAME_DISPATCHRECORDER                     DECLARE_ASCII("com.sun.star.comp.framework.DispatchRecorder"        )
#define IMPLEMENTATIONNAME_MAILTODISPATCHER                     DECLARE_ASCII("com.sun.star.comp.framework.MailToDispatcher"        )
#define IMPLEMENTATIONNAME_SERVICEHANDLER                       DECLARE_ASCII("com.sun.star.comp.framework.ServiceHandler"          )
#define IMPLEMENTATIONNAME_UIINTERACTIONHANDLER                 DECLARE_ASCII("com.sun.star.comp.uui.UUIInteractionHandler"         )
#define IMPLEMENTATIONNAME_SUBSTITUTEPATHVARIABLES              DECLARE_ASCII("com.sun.star.comp.framework.PathSubstitution"        )
#define IMPLEMENTATIONNAME_PATHSETTINGS                         DECLARE_ASCII("com.sun.star.comp.framework.PathSettings"            )
#define IMPLEMENTATIONNAME_JOBDISPATCH                          DECLARE_ASCII("com.sun.star.comp.framework.jobs.JobDispatch"        )
#define IMPLEMENTATIONNAME_DISPATCHHELPER                       DECLARE_ASCII("com.sun.star.comp.framework.services.DispatchHelper" )
#define IMPLEMENTATIONNAME_DETECTORFACTORY                      DECLARE_ASCII("com.sun.star.comp.framework.DetectorFactory"         )
#define IMPLEMENTATIONNAME_LAYOUTMANAGER                        DECLARE_ASCII("com.sun.star.comp.framework.LayoutManager"           )
#define IMPLEMENTATIONNAME_POPUPMENUCONTROLLERFACTORY           DECLARE_ASCII("com.sun.star.comp.framework.PopupMenuControllerFactory" )
#define IMPLEMENTATIONNAME_MACROSMENUCONTROLLER                   DECLARE_ASCII("com.sun.star.comp.framework.MacrosMenuController"      )
#define IMPLEMENTATIONNAME_FONTMENUCONTROLLER                   DECLARE_ASCII("com.sun.star.comp.framework.FontMenuController"      )
#define IMPLEMENTATIONNAME_FONTSIZEMENUCONTROLLER               DECLARE_ASCII("com.sun.star.comp.framework.FontSizeMenuController"  )
#define IMPLEMENTATIONNAME_FORMATMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.FormatMenuController"    )
#define IMPLEMENTATIONNAME_OBJECTMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.ObjectMenuController"    )
#define IMPLEMENTATIONNAME_HEADERMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.HeaderMenuController"    )
#define IMPLEMENTATIONNAME_FOOTERMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.FooterMenuController"    )
#define IMPLEMENTATIONNAME_CONTROLMENUCONTROLLER                DECLARE_ASCII("com.sun.star.comp.framework.ControlMenuController"   )
#define IMPLEMENTATIONNAME_UICOMMANDDESCRIPTION                 DECLARE_ASCII("com.sun.star.comp.framework.UICommandDescription"    )
#define IMPLEMENTATIONNAME_UIELEMENTFACTORYMANAGER              DECLARE_ASCII("com.sun.star.comp.framework.UIElementFactoryManager" )
#define IMPLEMENTATIONNAME_MODULEMANAGER                        DECLARE_ASCII("com.sun.star.comp.framework.ModuleManager"           )
#define IMPLEMENTATIONNAME_UICONFIGURATIONMANAGER               DECLARE_ASCII("com.sun.star.comp.framework.UIConfigurationManager"  )
#define IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER DECLARE_ASCII("com.sun.star.comp.framework.ModuleUIConfigurationManagerSupplier" )
#define IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGER         DECLARE_ASCII("com.sun.star.comp.framework.ModuleUIConfigurationManager" )
#define IMPLEMENTATIONNAME_MENUBARFACTORY                       DECLARE_ASCII("com.sun.star.comp.framework.MenuBarFactory"          )
#define IMPLEMENTATIONNAME_STARTMODULE                          DECLARE_ASCII("com.sun.star.comp.framework.BackingComp"             )
#define IMPLEMENTATIONNAME_WINDOWSTATECONFIGURATION             DECLARE_ASCII("com.sun.star.comp.framework.WindowStateConfiguration" )
#define IMPLEMENTATIONNAME_TOOLBARFACTORY                       DECLARE_ASCII("com.sun.star.comp.framework.ToolBarFactory"          )
#define IMPLEMENTATIONNAME_ADDONSTOOLBARFACTORY                 DECLARE_ASCII("com.sun.star.comp.framework.AddonsToolBarFactory"    )
#define IMPLEMENTATIONNAME_TOOLBARSMENUCONTROLLER               DECLARE_ASCII("com.sun.star.comp.framework.ToolBarsMenuController"  )
#define IMPLEMENTATIONNAME_TOOLBARCONTROLLERFACTORY             DECLARE_ASCII("com.sun.star.comp.framework.ToolBarControllerFactory"    )
#define IMPLEMENTATIONNAME_LICENSE                              DECLARE_ASCII("com.sun.star.comp.framework.License"             )
#define IMPLEMENTATIONNAME_RECENTFILESMENUCONTROLLER            DECLARE_ASCII("com.sun.star.comp.framework.RecentFilesMenuController" )
}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_H_
