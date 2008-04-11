/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: services.h,v $
 * $Revision: 1.47 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_H_
#define __FRAMEWORK_SERVICES_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>

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
#define SERVICENAME_UCBCONTENTBROKER                            DECLARE_ASCII("com.sun.star.ucb.UniversalContentBroker"             )
#define SERVICENAME_STATUSINDICATORFACTORY                      DECLARE_ASCII("com.sun.star.task.StatusIndicatorFactory"            )
#define SERVICENAME_UIINTERACTIONHANDLER                        DECLARE_ASCII("com.sun.star.task.InteractionHandler"                )
#define SERVICENAME_JOBEXECUTOR                                 DECLARE_ASCII("com.sun.star.task.JobExecutor"                       )
#define SERVICENAME_JOB                                         DECLARE_ASCII("com.sun.star.task.Job"                               )
#define SERVICENAME_DISPATCHRECORDERSUPPLIER                    DECLARE_ASCII("com.sun.star.frame.DispatchRecorderSupplier"         )
#define SERVICENAME_DISPATCHRECORDER                            DECLARE_ASCII("com.sun.star.frame.DispatchRecorder"                 )
#define SERVICENAME_SYSTEMSHELLEXECUTE                          DECLARE_ASCII("com.sun.star.system.SystemShellExecute"              )
#define SERVICENAME_PROTOCOLHANDLER                             DECLARE_ASCII("com.sun.star.frame.ProtocolHandler"                  )
#define SERVICENAME_SUBSTITUTEPATHVARIABLES                     DECLARE_ASCII("com.sun.star.util.PathSubstitution"                  )
#define SERVICENAME_PATHSETTINGS                                DECLARE_ASCII("com.sun.star.util.PathSettings"                      )
#define SERVICENAME_DISPATCHHELPER                              DECLARE_ASCII("com.sun.star.frame.DispatchHelper"                   )
#define SERVICENAME_DETECTORFACTORY                             DECLARE_ASCII("com.sun.star.document.ExtendedTypeDetectionFactory"  )
#define SERVICENAME_SCRIPTCONVERTER                             DECLARE_ASCII("com.sun.star.script.Converter"                       )
#define SERVICENAME_LAYOUTMANAGER                               DECLARE_ASCII("com.sun.star.frame.LayoutManager"             )
#define SERVICENAME_POPUPMENUCONTROLLERFACTORY                  DECLARE_ASCII("com.sun.star.frame.PopupMenuControllerFactory" )
#define SERVICENAME_POPUPMENUCONTROLLER                         DECLARE_ASCII("com.sun.star.frame.PopupMenuController"       )
#define SERVICENAME_UICOMMANDDESCRIPTION                        DECLARE_ASCII("com.sun.star.frame.UICommandDescription"      )
#define SERVICENAME_UIELEMENTFACTORYMANAGER                     DECLARE_ASCII("com.sun.star.ui.UIElementFactoryManager"      )
#define SERVICENAME_MODULEMANAGER                               DECLARE_ASCII("com.sun.star.frame.ModuleManager"             )
#define SERVICENAME_UICONFIGURATIONMANAGER                      DECLARE_ASCII("com.sun.star.ui.UIConfigurationManager"       )
#define SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER        DECLARE_ASCII("com.sun.star.ui.ModuleUIConfigurationManagerSupplier" )
#define SERVICENAME_MODULEUICONFIGURATIONMANAGER                DECLARE_ASCII("com.sun.star.ui.ModuleUIConfigurationManager" )
#define SERVICENAME_MENUBARFACTORY                              DECLARE_ASCII("com.sun.star.ui.UIElementFactory"             )
#define SERVICENAME_WINDOWSTATECONFIGURATION                    DECLARE_ASCII("com.sun.star.ui.WindowStateConfiguration"     )
#define SERVICENAME_STARTMODULE                                 DECLARE_ASCII("com.sun.star.frame.StartModule"                      )
#define SERVICENAME_FRAMECONTROLLER                             DECLARE_ASCII("com.sun.star.frame.Controller"                       )
#define SERVICENAME_GLOBALACCELERATORCONFIGURATION              DECLARE_ASCII("com.sun.star.ui.GlobalAcceleratorConfiguration")
#define SERVICENAME_MODULEACCELERATORCONFIGURATION              DECLARE_ASCII("com.sun.star.ui.ModuleAcceleratorConfiguration")
#define SERVICENAME_DOCUMENTACCELERATORCONFIGURATION            DECLARE_ASCII("com.sun.star.ui.DocumentAcceleratorConfiguration")
#define SERVICENAME_SAXPARSER                                   DECLARE_ASCII("com.sun.star.xml.sax.Parser"                         )
#define SERVICENAME_SAXWRITER                                   DECLARE_ASCII("com.sun.star.xml.sax.Writer"                         )
#define SERVICENAME_SIMPLEFILEACCESS                            DECLARE_ASCII("com.sun.star.ucb.SimpleFileAccess"                   )
#define SERVICENAME_STORAGEFACTORY                              DECLARE_ASCII("com.sun.star.embed.StorageFactory"                   )
#define SERVICENAME_FILESYSTEMSTORAGEFACTORY                    DECLARE_ASCII("com.sun.star.embed.FileSystemStorageFactory"         )
#define SERVICENAME_TEMPFILE                                    DECLARE_ASCII("com.sun.star.io.TempFile"                            )
#define SERVICENAME_PACKAGESTRUCTURECREATOR                     DECLARE_ASCII("com.sun.star.embed.PackageStructureCreator"          )
#define SERVICENAME_TOOLBARFACTORY                              DECLARE_ASCII("com.sun.star.ui.ToolBarFactory"               )
#define SERVICENAME_TOOLBARCONTROLLERFACTORY                    DECLARE_ASCII("com.sun.star.frame.ToolBarControllerFactory"         )
#define SERVICENAME_LICENSE                                     SERVICENAME_JOB
#define SERVICENAME_AUTORECOVERY                                DECLARE_ASCII("com.sun.star.frame.AutoRecovery"                     )
#define SERVICENAME_GLOBALEVENTBROADCASTER                      DECLARE_ASCII("com.sun.star.frame.GlobalEventBroadcaster"           )
#define SERVICENAME_STATUSBARFACTORY                            DECLARE_ASCII("com.sun.star.ui.StatusBarFactory"             )
#define SERVICENAME_UICATEGORYDESCRIPTION                       DECLARE_ASCII("com.sun.star.ui.UICategoryDescription"               )
#define SERVICENAME_STATUSBARCONTROLLERFACTORY                  DECLARE_ASCII("com.sun.star.frame.StatusbarControllerFactory"       )
#define SERVICENAME_SESSIONLISTENER                             DECLARE_ASCII("com.sun.star.frame.SessionListener"                  )
#define SERVICENAME_STATUSBARCONTROLLER                         DECLARE_ASCII("com.sun.star.frame.StatusbarController"              )
#define SERVICENAME_TABREG                                      DECLARE_ASCII("com.sun.star.tab.tabreg"                             )
#define SERVICENAME_TASKCREATOR                                 DECLARE_ASCII("com.sun.star.frame.TaskCreator"                      )
#define SERVICENAME_TABWINFACTORY                               DECLARE_ASCII("com.sun.star.frame.TabWindowFactory"                 )
#define SERVICENAME_TABWINDOW                                   DECLARE_ASCII("com.sun.star.frame.TabWindow"                        )
#define SERVICENAME_STRINGABBREVIATION                          DECLARE_ASCII("com.sun.star.util.UriAbbreviation"                   )
#define SERVICENAME_IMAGEMANAGER                                DECLARE_ASCII("com.sun.star.ui.ImageManager"                        )

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
#define IMPLEMENTATIONNAME_CONTENTHANDLERFACTORY                DECLARE_ASCII("com.sun.star.comp.framework.ContentHandlerFactory"   )
#define IMPLEMENTATIONNAME_SFXTERMINATOR                        DECLARE_ASCII("com.sun.star.comp.sfx2.SfxTerminateListener"         )
#define IMPLEMENTATIONNAME_PIPETERMINATOR                       DECLARE_ASCII("com.sun.star.comp.OfficeIPCThreadController"         )
#define IMPLEMENTATIONNAME_QUICKLAUNCHER                        DECLARE_ASCII("com.sun.star.comp.desktop.QuickstartWrapper"         )
#define IMPLEMENTATIONNAME_SWTHREADMANAGER                      DECLARE_ASCII("com.sun.star.util.comp.FinalThreadManager"           )
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
#define IMPLEMENTATIONNAME_MACROSMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.MacrosMenuController"      )
#define IMPLEMENTATIONNAME_FONTMENUCONTROLLER                   DECLARE_ASCII("com.sun.star.comp.framework.FontMenuController"      )
#define IMPLEMENTATIONNAME_FONTSIZEMENUCONTROLLER               DECLARE_ASCII("com.sun.star.comp.framework.FontSizeMenuController"  )
#define IMPLEMENTATIONNAME_FORMATMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.FormatMenuController"    )
#define IMPLEMENTATIONNAME_OBJECTMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.ObjectMenuController"    )
#define IMPLEMENTATIONNAME_HEADERMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.HeaderMenuController"    )
#define IMPLEMENTATIONNAME_FOOTERMENUCONTROLLER                 DECLARE_ASCII("com.sun.star.comp.framework.FooterMenuController"    )
#define IMPLEMENTATIONNAME_CONTROLMENUCONTROLLER                DECLARE_ASCII("com.sun.star.comp.framework.ControlMenuController"   )
#define IMPLEMENTATIONNAME_NEWMENUCONTROLLER                    DECLARE_ASCII("com.sun.star.comp.framework.NewMenuController"       )
#define IMPLEMENTATIONNAME_UICOMMANDDESCRIPTION                 DECLARE_ASCII("com.sun.star.comp.framework.UICommandDescription"    )
#define IMPLEMENTATIONNAME_UIELEMENTFACTORYMANAGER              DECLARE_ASCII("com.sun.star.comp.framework.UIElementFactoryManager" )
#define IMPLEMENTATIONNAME_MODULEMANAGER                        DECLARE_ASCII("com.sun.star.comp.framework.ModuleManager"           )
#define IMPLEMENTATIONNAME_UICONFIGURATIONMANAGER               DECLARE_ASCII("com.sun.star.comp.framework.UIConfigurationManager"  )
#define IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER DECLARE_ASCII("com.sun.star.comp.framework.ModuleUIConfigurationManagerSupplier" )
#define IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGER         DECLARE_ASCII("com.sun.star.comp.framework.ModuleUIConfigurationManager" )
#define IMPLEMENTATIONNAME_MENUBARFACTORY                       DECLARE_ASCII("com.sun.star.comp.framework.MenuBarFactory"          )
#define IMPLEMENTATIONNAME_STARTMODULE                          DECLARE_ASCII("com.sun.star.comp.framework.BackingComp"             )
#define IMPLEMENTATIONNAME_GLOBALACCELERATORCONFIGURATION       DECLARE_ASCII("com.sun.star.comp.framework.GlobalAcceleratorConfiguration")
#define IMPLEMENTATIONNAME_MODULEACCELERATORCONFIGURATION       DECLARE_ASCII("com.sun.star.comp.framework.ModuleAcceleratorConfiguration")
#define IMPLEMENTATIONNAME_DOCUMENTACCELERATORCONFIGURATION     DECLARE_ASCII("com.sun.star.comp.framework.DocumentAcceleratorConfiguration")
#define IMPLEMENTATIONNAME_WINDOWSTATECONFIGURATION             DECLARE_ASCII("com.sun.star.comp.framework.WindowStateConfiguration" )
#define IMPLEMENTATIONNAME_TOOLBARFACTORY                       DECLARE_ASCII("com.sun.star.comp.framework.ToolBarFactory"          )
#define IMPLEMENTATIONNAME_ADDONSTOOLBARFACTORY                 DECLARE_ASCII("com.sun.star.comp.framework.AddonsToolBarFactory"    )
#define IMPLEMENTATIONNAME_TOOLBARSMENUCONTROLLER               DECLARE_ASCII("com.sun.star.comp.framework.ToolBarsMenuController"  )
#define IMPLEMENTATIONNAME_TOOLBARCONTROLLERFACTORY             DECLARE_ASCII("com.sun.star.comp.framework.ToolBarControllerFactory"    )
#define IMPLEMENTATIONNAME_LICENSE                              DECLARE_ASCII("com.sun.star.comp.framework.License"             )
#define IMPLEMENTATIONNAME_AUTORECOVERY                         DECLARE_ASCII("com.sun.star.comp.framework.AutoRecovery"            )
#define IMPLEMENTATIONNAME_STATUSINDICATORFACTORY               DECLARE_ASCII("com.sun.star.comp.framework.StatusIndicatorFactory"  )
#define IMPLEMENTATIONNAME_RECENTFILESMENUCONTROLLER            DECLARE_ASCII("com.sun.star.comp.framework.RecentFilesMenuController" )
#define IMPLEMENTATIONNAME_STATUSBARFACTORY                     DECLARE_ASCII("com.sun.star.comp.framework.StatusBarFactory"        )
#define IMPLEMENTATIONNAME_STATUSBARCONTROLLERFACTORY           DECLARE_ASCII("com.sun.star.comp.framework.StatusBarControllerFactory" )
#define IMPLEMENTATIONNAME_UICATEGORYDESCRIPTION                DECLARE_ASCII("com.sun.star.comp.framework.UICategoryDescription"   )
#define IMPLEMENTATIONNAME_APPDISPATCHPROVIDER                  DECLARE_ASCII("com.sun.star.comp.sfx2.AppDispatchProvider"          )
#define IMPLEMENTATIONNAME_SESSIONLISTENER                      DECLARE_ASCII("com.sun.star.comp.frame.SessionListener"             )
#define IMPLEMENTATIONNAME_HELPONSTARTUP                        DECLARE_ASCII("com.sun.star.comp.framework.HelpOnStartup"           )
#define IMPLEMENTATIONNAME_SHELLJOB                             DECLARE_ASCII("com.sun.star.comp.framework.ShellJob"                )
#define IMPLEMENTATIONNAME_LOGOIMAGESTATUSBARCONTROLLER         DECLARE_ASCII("com.sun.star.comp.framework.LogoImageStatusbarController" )
#define IMPLEMENTATIONNAME_LOGOTEXTSTATUSBARCONTROLLER          DECLARE_ASCII("com.sun.star.comp.framework.LogoTextStatusbarController" )
#define IMPLEMENTATIONNAME_FWK_TASKCREATOR                      DECLARE_ASCII("com.sun.star.comp.framework.TaskCreator"                 )
#define IMPLEMENTATIONNAME_TABWINFACTORY                        DECLARE_ASCII("com.sun.star.comp.framework.TabWindowFactory"        )
#define IMPLEMENTATIONNAME_TABWINDOW                            DECLARE_ASCII("com.sun.star.comp.framework.TabWindow"               )
#define IMPLEMENTATIONNAME_SYSTEMEXEC                           DECLARE_ASCII("com.sun.star.comp.framework.SystemExecute"           )
#define IMPLEMENTATIONNAME_SIMPLETEXTSTATUSBARCONTROLLER        DECLARE_ASCII("com.sun.star.comp.framework.SimpleTextStatusbarController" )
#define IMPLEMENTATIONNAME_URIABBREVIATION                      DECLARE_ASCII("com.sun.star.comp.framework.UriAbbreviation"         )
#define IMPLEMENTATIONNAME_POPUPMENUDISPATCHER                  DECLARE_ASCII("com.sun.star.comp.framework.PopupMenuControllerDispatcher" )
#define IMPLEMENTATIONNAME_LANGSELECTIONSTATUSBARCONTROLLER     DECLARE_ASCII("com.sun.star.comp.framework.LangSelectionStatusbarController"  )
#define IMPLEMENTATIONNAME_LANGUAGESELECTIONMENUCONTROLLER      DECLARE_ASCII("com.sum.star.comp.framework.LanguageSelectionMenuController" )
#define IMPLEMENTATIONNAME_IMAGEMANAGER                         DECLARE_ASCII("com.sun.star.comp.framework.ImageManager"            )

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_H_
