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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_H
#define INCLUDED_FRAMEWORK_INC_SERVICES_H

namespace framework{

//  used servicenames by framework

#define SERVICENAME_FILTERFACTORY                               "com.sun.star.document.FilterFactory"
#define SERVICENAME_CFGUPDATEACCESS                             "com.sun.star.configuration.ConfigurationUpdateAccess"   // provides read/write access to the configuration
#define SERVICENAME_CFGREADACCESS                               "com.sun.star.configuration.ConfigurationAccess"         // provides readonly access to the configuration
#define SERVICENAME_CONTENTHANDLER                              "com.sun.star.frame.ContentHandler"
#define SERVICENAME_JOB                                         "com.sun.star.task.Job"
#define SERVICENAME_PROTOCOLHANDLER                             "com.sun.star.frame.ProtocolHandler"
#define SERVICENAME_POPUPMENUCONTROLLER                         "com.sun.star.frame.PopupMenuController"
#define SERVICENAME_STRINGABBREVIATION                          "com.sun.star.util.UriAbbreviation"

//  used implementationnames by framework

#define IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER             "com.sun.star.comp.framework.MediaTypeDetectionHelper"
#define IMPLEMENTATIONNAME_QUICKLAUNCHER                        "com.sun.star.comp.desktop.QuickstartWrapper"
#define IMPLEMENTATIONNAME_DISPATCHRECORDERSUPPLIER             "com.sun.star.comp.framework.DispatchRecorderSupplier"
#define IMPLEMENTATIONNAME_MAILTODISPATCHER                     "com.sun.star.comp.framework.MailToDispatcher"
#define IMPLEMENTATIONNAME_MACROSMENUCONTROLLER                 "com.sun.star.comp.framework.MacrosMenuController"
#define IMPLEMENTATIONNAME_FONTMENUCONTROLLER                   "com.sun.star.comp.framework.FontMenuController"
#define IMPLEMENTATIONNAME_FONTSIZEMENUCONTROLLER               "com.sun.star.comp.framework.FontSizeMenuController"
#define IMPLEMENTATIONNAME_HEADERMENUCONTROLLER                 "com.sun.star.comp.framework.HeaderMenuController"
#define IMPLEMENTATIONNAME_FOOTERMENUCONTROLLER                 "com.sun.star.comp.framework.FooterMenuController"
#define IMPLEMENTATIONNAME_FWK_TASKCREATOR                      "com.sun.star.comp.framework.TaskCreator"
#define IMPLEMENTATIONNAME_LANGUAGESELECTIONMENUCONTROLLER      "com.sun.star.comp.framework.LanguageSelectionMenuController"

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
