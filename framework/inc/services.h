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

#pragma once

#include <rtl/ustring.hxx>

namespace framework{

//  used servicenames by framework

inline constexpr OUStringLiteral SERVICENAME_FILTERFACTORY = u"com.sun.star.document.FilterFactory";
inline constexpr OUStringLiteral SERVICENAME_CFGUPDATEACCESS = u"com.sun.star.configuration.ConfigurationUpdateAccess"; // provides read/write access to the configuration
inline constexpr OUStringLiteral SERVICENAME_CFGREADACCESS = u"com.sun.star.configuration.ConfigurationAccess";  // provides readonly access to the configuration
inline constexpr OUStringLiteral SERVICENAME_JOB = u"com.sun.star.task.Job";
inline constexpr OUStringLiteral SERVICENAME_PROTOCOLHANDLER = u"com.sun.star.frame.ProtocolHandler";
inline constexpr OUStringLiteral SERVICENAME_POPUPMENUCONTROLLER = u"com.sun.star.frame.PopupMenuController";

//  used implementationnames by framework

#define IMPLEMENTATIONNAME_QUICKLAUNCHER                        "com.sun.star.comp.desktop.QuickstartWrapper"
inline constexpr OUStringLiteral IMPLEMENTATIONNAME_FWK_TASKCREATOR = u"com.sun.star.comp.framework.TaskCreator";

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
